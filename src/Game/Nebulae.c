/*=============================================================================
    Name    : nebulae.c
    Purpose : management and display of nebulae

    Created 3/9/1998 by khent
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifndef SW_Render
#ifdef _WIN32
#include <windows.h>
#endif
#endif
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "Types.h"
#include "Memory.h"
#include "Clouds.h"
#include "Randy.h"
#include "glinc.h"
#include "FastMath.h"
#include "render.h"
#include "Debug.h"
#include "UnivUpdate.h"
#include "StatScript.h"
#include "Matrix.h"
#include "Universe.h"
#include "Nebulae.h"
#include "mainrgn.h"
#include "AutoLOD.h"

#include "Shader.h"
#include "glcaps.h"

#include "SaveGame.h"

ubyte nebColor[4];

#define NEB_RENDER RGL_SPECULAR2_RENDER

#define TENDRILBRIGHTEN 60

#define TENDRILCOLOR0(a) \
    { \
        ubyte b = a; \
        if (!_bright) \
        { \
            b += TENDRILBRIGHTEN; \
        } \
        nebColor[0] = (ubyte)(NEB_FOG_RED * 255.0f); \
        nebColor[1] = (ubyte)(NEB_FOG_GREEN * 255.0f); \
        nebColor[2] = (ubyte)(NEB_FOG_BLUE * 255.0f); \
        nebColor[3] = b; \
        glColor4ub(nebColor[0], nebColor[1], nebColor[2], nebColor[3]); \
    }
#define TENDRILCOLOR(t,a) \
    { \
        ubyte b = a; \
        if (!_bright) \
        { \
            b += TENDRILBRIGHTEN; \
        } \
        nebColor[0] = colRed(t->colour); \
        nebColor[1] = colGreen(t->colour); \
        nebColor[2] = colBlue(t->colour); \
        nebColor[3] = b; \
        glColor4ub(nebColor[0], nebColor[1], nebColor[2], nebColor[3]); \
    }

// -----
// DATA
// -----

static bool _bright = TRUE;

static ubyte TENDRILALPHA = 163;

#define NEB_CHUNK_RADIUS 800.0f

static real32 nebFogColor[4];

real32 nebFogSum = 0.0f;

static udword _alloc;

static udword neb_frame_counter;

real32 NEB_RADIUS = 100.0f;

//initialize to reasonable defaults
static real32 NEB_FOG_RED = 0.1f;
static real32 NEB_FOG_GREEN = 0.3f;
static real32 NEB_FOG_BLUE = 0.7f;
static real32 NEB_BG_SCALAR = 0.2f;

static real32 NEB_TENDRIL_RED_DEV = 0.05f;
static real32 NEB_TENDRIL_GREEN_DEV = 0.05f;
static real32 NEB_TENDRIL_BLUE_DEV = 0.1f;

static real32 NEB_DISTANCE_DESATURATION = 127.0f;

static real32 NEB_MAX_TENDRIL_ANGULAR_DEVIATION = 45.0f;
static real32 NEB_MAX_TENDRIL_DISTANCE = 80000.0f;
static real32 NEB_PICK_DISTANCE_THRESHOLD = 12000.0f;
static real32 NEB_PICK_DISTANCE_MINTHRESH = 500.0f;

static real32 NEB_SHUFFLE_DISTANCE = 550.0f;

static real32 NEB_VEL_RAN_MULTIPLIER = 0.4f;
static real32 NEB_RAN_RANGE = 3.0f;
static real32 NEB_VEL_BASE = 0.4f;

static real32 NEB_TENDRIL_RADIUS_BASE = 200.0f;
static real32 NEB_TENDRIL_RADIUS_RANGE = 400.0f;

scriptEntry NebulaeTweaks[] =
{
    makeEntry(NEB_RADIUS, scriptSetReal32CB),
    makeEntry(NEB_FOG_RED, scriptSetReal32CB),
    makeEntry(NEB_FOG_GREEN, scriptSetReal32CB),
    makeEntry(NEB_FOG_BLUE, scriptSetReal32CB),
    makeEntry(NEB_BG_SCALAR, scriptSetReal32CB),
    makeEntry(NEB_TENDRIL_RED_DEV, scriptSetReal32CB),
    makeEntry(NEB_TENDRIL_GREEN_DEV, scriptSetReal32CB),
    makeEntry(NEB_TENDRIL_BLUE_DEV, scriptSetReal32CB),
    makeEntry(NEB_DISTANCE_DESATURATION, scriptSetReal32CB),
    makeEntry(NEB_MAX_TENDRIL_ANGULAR_DEVIATION, scriptSetReal32CB),
    makeEntry(NEB_MAX_TENDRIL_DISTANCE, scriptSetReal32CB),
    makeEntry(NEB_PICK_DISTANCE_THRESHOLD, scriptSetReal32CB),
    makeEntry(NEB_PICK_DISTANCE_MINTHRESH, scriptSetReal32CB),
    makeEntry(NEB_SHUFFLE_DISTANCE, scriptSetReal32CB),
    makeEntry(NEB_VEL_RAN_MULTIPLIER, scriptSetReal32CB),
    makeEntry(NEB_RAN_RANGE, scriptSetReal32CB),
    makeEntry(NEB_VEL_BASE, scriptSetReal32CB),
    makeEntry(NEB_TENDRIL_RADIUS_BASE, scriptSetReal32CB),
    makeEntry(NEB_TENDRIL_RADIUS_RANGE, scriptSetReal32CB),
    endEntry
};

nebulae_t nebNebulae[NEB_MAX_NEBULAE];
udword numNebulae = 0;

void nebRenderNebula(nebulae_t* neb);


// -----
// HELPERS
// -----

real32 realRand(real32 n)
{
    if (n == 0.0f)
    {
        return 0.0f;
    }
    else
    {
        return((real32)(ranRandom(RAN_Nebulae) % (sdword)(n * 10000.0f)) / 10000.0f);
    }
}

#if 1
real32 nebRealDist(real32 n, real32 d)
{
    real32 twod = 2.0f*d;
    return(n + (realRand(twod) - d));
}
#else
real32 nebRealDist(real32 n, real32 d)
{
    real32 r = (real32)((real64)(ranRandom(RAN_Nebulae) % 100000) / 100000.0);
    real32 sign = (ranRandom(RAN_Nebulae) % 2 == 0) ? -1.0f : 1.0f;
    if (d < 0.0f)
    {
        sign = -1.0f;
    }
    return n + sign*r*d;
}
#endif

real32 _fabs(real32 n)
{
    return (n < 0.0f) ? -n : n;
}

static real32 fsin(real32 a)
{
    return (real32)sin((double)a);
}

static real32 fcos(real32 a)
{
    return (real32)cos((double)a);
}

void nebHomogenize(hvector* h)
{
    real32 oneOverW;

    if (h->w == 0.0f || h->w == 1.0f)
        return;

    oneOverW = 1.0f / h->w;
    h->x *= oneOverW;
    h->y *= oneOverW;
    h->z *= oneOverW;
}

static void _clearcolorUniverse()
{
    color c = universe.backgroundColor;
    if (!smSensorsActive)
    {
        rndSetClearColor(colRGBA(colRed(c),
                                 colGreen(c),
                                 colBlue(c),
                                 255));
    }
}


// -----
// FUNCTIONS
// -----

void nebColorInit()
{
    nebFogColor[0] = NEB_FOG_RED;
    nebFogColor[1] = NEB_FOG_GREEN;
    nebFogColor[2] = NEB_FOG_BLUE;
    nebFogColor[3] = 1.0f;
}

/*-----------------------------------------------------------------------------
    Name        : nebStartup
    Description : initializes necessary things.  different from actually creating
                  a chunk distribution and linking 'em up with tendrils, however
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void nebStartup()
{
    sdword i;

    neb_frame_counter = 0;

    _alloc = 0;

    for (i = 0; i < NEB_MAX_NEBULAE; i++)
    {
        nebNebulae[i].numChunks = 0;
        nebNebulae[i].numTendrils = 0;
        nebNebulae[i].chunkTable = NULL;
        nebNebulae[i].tendrilTable = NULL;
    }

    ranParametersReset(RAN_Nebulae);

    nebColorInit();

    _bright = glCapFastFeature(GL_BLEND);
}

/*-----------------------------------------------------------------------------
    Name        : nebReset
    Description : resets nebulae
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void nebReset()
{
    ranParametersReset(RAN_Nebulae);
    nebShutdown();
    _clearcolorUniverse();
    rndFogOn = FALSE;
}

/*-----------------------------------------------------------------------------
    Name        : nebShutdown
    Description : frees memory &c
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void nebShutdown()
{
    sdword i;

    if (numNebulae > 0)
    {
        for (i = 0; i < numNebulae; i++)
        {
            nebFreeChunks(&nebNebulae[i]);
            nebFreeTendrils(&nebNebulae[i]);
        }

        numNebulae = 0;
    }
#if 0
    if (_alloc > 0)
    {
        FILE* out = fopen("neb.dat", "wt");
        if (out != NULL)
        {
            fprintf(out, "bytes %d (%dK)\n", _alloc, _alloc >> 10);
            fclose(out);
        }
    }
#endif
    _alloc = 0;
}

/*-----------------------------------------------------------------------------
    Name        : nebAllocateChunks
    Description : allocates memory for the global chunk table
    Inputs      : num - number of chunks to create space for
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void nebAllocateChunks(nebulae_t* neb, udword num)
{
    if (neb->chunkTable != NULL)
    {
        nebFreeChunks(neb);
    }

    dbgAssert(num > 1);

    neb->chunkTable = memAlloc(num * sizeof(nebChunk), "nebula chunkTable", 0);
    neb->numChunks = num;
}

/*-----------------------------------------------------------------------------
    Name        : nebAllocateTendrils
    Description : allocates memory for the global tendril table
    Inputs      : num - number of tendrils to create space for
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void nebAllocateTendrils(nebulae_t* neb, udword num)
{
    udword i;
    nebTendril* tendril;

    dbgAssert(num > 0);

    if (neb->tendrilTable != NULL)
    {
        nebFreeTendrils(neb);
    }

    neb->tendrilTable = memAlloc(num * sizeof(nebTendril), "nebula tendrilTable", 0);
    _alloc += num * sizeof(nebTendril);
    neb->numTendrils = num;

    for (i = 0, tendril = neb->tendrilTable; i < neb->numTendrils; i++, tendril++)
    {
        nebInitTendril(tendril);
    }
}

/*-----------------------------------------------------------------------------
    Name        : nebFreeChunks
    Description : frees memory allocated to nebChunks in the chunkTable
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void nebFreeChunks(nebulae_t* neb)
{
    if (neb->chunkTable != NULL)
    {
        memFree(neb->chunkTable);
        neb->chunkTable = NULL;
        neb->numChunks = 0;
    }
}

/*-----------------------------------------------------------------------------
    Name        : nebFreeTendrils
    Description : frees memory allocated to nebTendrils in the tendrilTable
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void nebFreeTendrils(nebulae_t* neb)
{
    udword i, j;
    nebTendril* tendril;

    if (neb->tendrilTable != NULL)
    {
        for (i = 0, tendril = neb->tendrilTable; i < neb->numTendrils; i++, tendril++)
        {
            for (j = 0; j < NUM_NEBTENDRIL_LODS; j++)
            {
                if (tendril->lod[j].vertices != NULL)
                {
                    memFree(tendril->lod[j].vertices);
                    tendril->lod[j].vertices = NULL;
                    tendril->lod[j].numVerts = 0;
                }
                if (tendril->lod[j].normals != NULL)
                {
                    memFree(tendril->lod[j].normals);
                    tendril->lod[j].normals = NULL;
                }
                if (tendril->lod[j].faces != NULL)
                {
                    memFree(tendril->lod[j].faces);
                    tendril->lod[j].faces = NULL;
                    tendril->lod[j].numFaces = 0;
                }
            }
        }

        memFree(neb->tendrilTable);
        neb->tendrilTable = NULL;
        neb->numTendrils = 0;
    }
}

/*-----------------------------------------------------------------------------
    Name        : chunkRandomVelocity
    Description : applies a random velocity to a chunk
    Inputs      : the chunk
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void chunkRandomVelocity(nebChunk* chunk)
{
    real32 chunkVelocity = NEB_VEL_RAN_MULTIPLIER*(real32)(ranRandom(RAN_Nebulae) % (udword)NEB_RAN_RANGE) + NEB_VEL_BASE;

    dbgAssert(chunk != NULL);
    cloudRandomSphericalPoint(&chunk->velocity);
    vecMultiplyByScalar(chunk->velocity, chunkVelocity);
}

/*-----------------------------------------------------------------------------
    Name        : nebInitChunk
    Description : inits a blank chunk
    Inputs      : chunk - the chunk to init
    Outputs     : an init'ed chunk
    Return      :
----------------------------------------------------------------------------*/
void nebInitChunk(nebChunk* chunk)
{
    udword i;

    dbgAssert(chunk != NULL);

    chunk->flags = 0;
    chunk->counter = 0;
    chunk->numTendrils = 0;

    chunk->innerRadius = 160.0f;
    chunk->outerRadius = 245.0f;
    vecSet(chunk->position, 0.0f, 0.0f, 0.0f);

    chunkRandomVelocity(chunk);
    vecSet(chunk->dPos, 0.0f, 0.0f, 0.0f);

    for (i = 0; i < NEB_MAX_TENDRILS; i++)
    {
        chunk->tendrils[i] = NULL;
        chunk->tFlags[i] = 0;
    }
}

/*-----------------------------------------------------------------------------
    Name        : nebInitTendril
    Description : inits a blank tendril
    Inputs      : tendril - the tendril to init
    Outputs     : an init'ed tendril
    Return      :
----------------------------------------------------------------------------*/
void nebInitTendril(nebTendril* tendril)
{
    udword i;

    dbgAssert(tendril != NULL);

    tendril->flags = 0;
    tendril->counter = 0;
    tendril->a = NULL;
    tendril->b = NULL;
    tendril->prev = NULL;
    tendril->next = NULL;
    vecSet(tendril->direction, 0.0f, 0.0f, 0.0f);

    for (i = 0; i < NUM_NEBTENDRIL_LODS; i++)
    {
        tendril->lod[i].numVerts = 0;
        tendril->lod[i].numFaces = 0;
        tendril->lod[i].vertices = NULL;
        tendril->lod[i].normals = NULL;
        tendril->lod[i].faces = NULL;
    }

    {
        sdword r, g, b, a;

        r = (sdword)(255.0f * nebRealDist(NEB_FOG_RED, NEB_TENDRIL_RED_DEV));
        g = (sdword)(255.0f * nebRealDist(NEB_FOG_GREEN, NEB_TENDRIL_GREEN_DEV));
        b = (sdword)(255.0f * nebRealDist(NEB_FOG_BLUE, NEB_TENDRIL_BLUE_DEV));
        a = TENDRILALPHA;

        if (r > 255) r = 255;
        else if (r < 0) r = 0;
        if (g > 255) g = 255;
        else if (g < 0) g = 0;
        if (b > 255) b = 255;
        else if (b < 0) b = 0;

        tendril->realColour = colRGBA(r,g,b,a);
        tendril->maxAlpha = (ubyte)a;
    }

    tendril->fadeFactor = 1.0f;
}

/*-----------------------------------------------------------------------------
    Name        : nebDistributeChunks
    Description : throws out a bunch of chunks
    Inputs      : origin - centre of distribution
                  radius - maximum extent of a chunk wrt origin
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void nebDistributeChunks(nebulae_t* neb, vector* origin, vector* dimension)
{
    udword i;
    nebChunk* chunk;

    for (i = 0, chunk = neb->chunkTable; i < neb->numChunks; i++, chunk++)
    {
        nebInitChunk(chunk);

        chunk->position.x = realRand(dimension->x) - 0.5f*dimension->x;
        chunk->position.y = realRand(dimension->y) - 0.5f*dimension->y;
        chunk->position.z = realRand(dimension->z) - 0.5f*dimension->z;
        vecAddTo(chunk->position, *origin);

        //radius of chunk left at default for now
    }
}

/*-----------------------------------------------------------------------------
    Name        : nebGetFreshTendril
    Description : returns a free tendril freshly initialized, if it can
    Inputs      :
    Outputs     :
    Return      : a freshly initialized tendril, or NULL
----------------------------------------------------------------------------*/
nebTendril* nebGetFreshTendril(nebulae_t* neb)
{
    udword i;
    nebTendril* tendril;

    for (i = 0, tendril = neb->tendrilTable; i < neb->numTendrils; i++, tendril++)
    {
        if (tendril->a == NULL && tendril->b == NULL)
        {
            nebInitTendril(tendril);
            return tendril;
        }
    }

    return NULL;
}

/*-----------------------------------------------------------------------------
    Name        : nebNumSimilarTendrils
    Description : returns the number of active tendrils which have == endpoints
    Inputs      : chunka, chunkb - endpoints
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
udword nebNumSimilarTendrils(nebulae_t* neb, nebChunk* chunka, nebChunk* chunkb)
{
    udword i, hits;
    nebTendril* tendril;

    for (i = hits = 0, tendril = neb->tendrilTable; i < neb->numTendrils; i++, tendril++)
    {
        if ((tendril->a == chunka && tendril->b == chunkb) ||
            (tendril->a == chunkb && tendril->b == chunka))
        {
            hits++;
        }
    }

    return hits;
}

/*-----------------------------------------------------------------------------
    Name        : nebAngularDeviation
    Description : returns the angular deviation from previous path given a new chunk
    Inputs      : newChunk - the possible next step in the tendril path (new endpoint)
                  oldChunka - previous startpoint
                  oldChunkb - previous endpoint
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
real32 nebAngularDeviation(nebChunk* newChunk, nebChunk* oldChunka, nebChunk* oldChunkb)
{
    vector lastVec;
    vector nextVec;
    real32 val;

    vecSub(lastVec, oldChunkb->position, oldChunka->position);
    vecSub(nextVec, newChunk->position, oldChunkb->position);
    vecNormalize(&lastVec);
    vecNormalize(&nextVec);

    val = vecDotProduct(nextVec, lastVec);
    return _fabs(RAD_TO_DEG((real32)acos((real64)val)));
}

/*-----------------------------------------------------------------------------
    Name        : nebPickChunk
    Description : picks a new chunk out of the chunkTable subject to the constraints
                  of the given chunk type (leading chunk or otherwise) and the
                  possibly provided previous chunk(s)
    Inputs      : type - NEB_INITIAL_CHUNK or NEB_MEDIAN_CHUNK
                  constrainta, constraintb - either a chunk or NULL
    Outputs     :
    Return      : a desired chunk or NULL
----------------------------------------------------------------------------*/
/*
 * Logic Description:
 *
 * a chunk is "good" at the initial stage if it has < max tendrils.
 * thereafter a chunk is "good" if it doesn't deviate more than a certain threshold
 * from a straight line.  a tendril would rather die than deviate
 *
 * initial chunks may be selected with a weighting favouring a tendrilled chunk.
 *
 * chunk-picking logic: gather a few chunks, weight un/favourably wrt numTendrils &
 * distance, select only one
 *
 * absolutely won't return a chunk if the chunk doesn't have a free tendril slot
 *
 * should have a separate mode given only 1 constraint: distance limiting
 */
nebChunk* nebPickChunk(nebulae_t* neb, sdword type, nebChunk* constrainta, nebChunk* constraintb)
{
#define NUM_CHUNKS 16
    nebChunk* chunk = NULL;
    nebChunk* chunks[NUM_CHUNKS];
    udword onChunk = 0;
    real32 minDist = 1e20f;
    real32 tempDist;
    udword i, maxTendrils;

    switch (type)
    {
    case NEB_INITIAL_CHUNK:
        for (i = 0, chunk = neb->chunkTable; i < neb->numChunks; i++, chunk++)
        {
            if (bitTest(chunk->flags, NEB_CHUNK_PICKED))
            {
                continue;
            }
            if (chunk->numTendrils + 1 < NEB_MAX_TENDRILS)
            {
                chunks[onChunk++] = chunk;
                if (onChunk == NUM_CHUNKS)
                {
                    break;
                }
            }
        }

        if (onChunk == 0)
        {
            return NULL;
        }

        chunk = chunks[0];
        maxTendrils = chunks[0]->numTendrils;
        for (i = 0/*silly*/; i < onChunk; i++)
        {
            if (chunks[i]->numTendrils > maxTendrils)
            {
                chunk = chunks[i];
                maxTendrils = chunks[i]->numTendrils;
            }
        }

        //the most tendrilled chunk we found
        return chunk;

    case NEB_MEDIAN_CHUNK:
        if (constrainta == NULL)
        {
            for (i = 0, chunk = neb->chunkTable; i < neb->numChunks; i++, chunk++)
            {
                if (bitTest(chunk->flags, NEB_CHUNK_PICKED))
                {
                    continue;
                }
                if (chunk->numTendrils + 1 < NEB_MAX_TENDRILS)
                {
                    return chunk;
                }
            }
        }
        else if (constraintb == NULL)
        {
            for (i = 0, chunk = neb->chunkTable; i < neb->numChunks; i++, chunk++)
            {
                if (bitTest(chunk->flags, NEB_CHUNK_PICKED))
                {
                    continue;
                }
                if ((chunk->numTendrils + 1 < NEB_MAX_TENDRILS) &&
                    (nebChunkDistance(constrainta, chunk) < NEB_PICK_DISTANCE_THRESHOLD) &&
                    (nebNumSimilarTendrils(neb, chunk, constrainta) == 0))
                {
                    return chunk;
                }
            }
        }
        else
        {
            //pick a few, and return the closest w/in straightness constraint
            for (i = 0, chunk = neb->chunkTable; i < neb->numChunks; i++, chunk++)
            {
                if (bitTest(chunk->flags, NEB_CHUNK_PICKED))
                {
                    continue;
                }
                if (chunk->numTendrils + 1 < NEB_MAX_TENDRILS)
                {
                    tempDist = nebChunkDistance(constraintb, chunk);

                    if ((nebAngularDeviation(chunk, constrainta, constraintb)
                        < NEB_MAX_TENDRIL_ANGULAR_DEVIATION) &&
                        (tempDist < NEB_PICK_DISTANCE_THRESHOLD) &&
                        (tempDist > NEB_PICK_DISTANCE_MINTHRESH) &&
                        (nebNumSimilarTendrils(neb, chunk, constraintb) == 0))
                    {
                        chunks[onChunk++] = chunk;
                        if (onChunk == NUM_CHUNKS)
                        {
                            break;
                        }
                    }
                }
            }

            if (onChunk == 0)
            {
                return NULL;
            }

            //locate nearest chunk
            for (i = 0; i < onChunk; i++)
            {
                tempDist = nebChunkDistance(chunks[i], constraintb);
                if (tempDist != 0.0f && tempDist < minDist)
                {
                    minDist = tempDist;
                    chunk = chunks[i];
                }
            }
            return chunk;
        }
        return NULL;

    default:
        return NULL;
    }
#undef NUM_CHUNKS
}

/*-----------------------------------------------------------------------------
    Name        : nebChunkDistance
    Description : returns the distance between 2 chunks
    Inputs      : a, b - the chunks
    Outputs     :
    Return      : a real value representing distance
----------------------------------------------------------------------------*/
real32 nebChunkDistance(nebChunk* a, nebChunk* b)
{
    if (a == NULL || b == NULL)
    {
        return 0.0f;
    }
    else
    {
        vector dist;
        vecSub(dist, a->position, b->position);
        return fsqrt(vecMagnitudeSquared(dist));
    }
}

/*-----------------------------------------------------------------------------
    Name        : nebChunkPicked
    Description : set a flag indicating whether a chunk is available for picking
    Inputs      : chunk - the chunk to set/clear the flag on
                  v - the boolean value
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void nebChunkPicked(nebChunk* chunk, bool8 v)
{
    dbgAssert(chunk != NULL);

    if (v)
    {
        bitSet(chunk->flags, NEB_CHUNK_PICKED);
    }
    else
    {
        bitClear(chunk->flags, NEB_CHUNK_PICKED);
    }
}

/*-----------------------------------------------------------------------------
    Name        : nebUnpickAllChunks
    Description : unpicks all chunks
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void nebUnpickAllChunks(nebulae_t* neb)
{
    udword i;
    nebChunk* chunk;

    for (i = 0, chunk = neb->chunkTable; i < neb->numChunks; i++, chunk++)
    {
        nebChunkPicked(chunk, FALSE);
    }
}

/*-----------------------------------------------------------------------------
    Name        : nebAddTendril
    Description : adds a tendril to a chunk
    Inputs      : chunk - the chunk to add the tendril to
                  tendril - the tendril to add
                  flags - NEB_CHUNK_FORE or NEB_CHUNK_AFT
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void nebAddTendril(nebChunk* chunk, nebTendril* tendril, udword flags)
{
    dbgAssert(chunk != NULL);

    chunk->tFlags[chunk->numTendrils] = flags;
    chunk->tendrils[chunk->numTendrils++] = (void*)tendril;
}

/*-----------------------------------------------------------------------------
    Name        : nebTendrilDirection
    Description : precalculates the direction the tendril goes
    Inputs      : tendril - the tendril
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void nebTendrilDirection(nebTendril* tendril)
{
    dbgAssert(tendril != NULL);
    dbgAssert(tendril->a != NULL);
    dbgAssert(tendril->b != NULL);
    vecSub(tendril->direction, tendril->b->position, tendril->a->position);
    vecNormalize(&tendril->direction);
}

/*-----------------------------------------------------------------------------
    Name        : nebAttachTendrils
    Description : attaches tendrils to bare chunks
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void nebAttachTendrils(nebulae_t* neb)
{
    nebChunk* chunka;
    nebChunk* chunkb;
    nebChunk* lastchunk;
    nebTendril* tendril;
    nebTendril* lastTendril;
    real32 distanceLeft;
    udword count = 0;

    for (;;)
    {
        nebUnpickAllChunks(neb);

        //attempt to get a fresh tendril
        tendril = nebGetFreshTendril(neb);
        if (tendril == NULL)
        {
            //no tendrils left, our work is done
            break;
        }

        //pick initial 2 chunks
REPICK:
        chunka = nebPickChunk(neb, NEB_INITIAL_CHUNK, NULL, NULL);
        nebChunkPicked(chunka, TRUE);
        chunkb = nebPickChunk(neb, NEB_MEDIAN_CHUNK, chunka, NULL);
        if (chunkb == NULL)
        {
            dbgAssert(++count < 100);
//            nebChunkPicked(chunka, FALSE);
            goto REPICK;
        }
        count = 0;
        nebChunkPicked(chunkb, TRUE);
        nebAddTendril(chunka, tendril, NEB_CHUNK_FORE);
        nebAddTendril(chunkb, tendril, NEB_CHUNK_AFT);

        //setup the tendril
        tendril->a = chunka;
        tendril->b = chunkb;
        nebTendrilDirection(tendril);
        lastTendril = tendril;
        bitSet(tendril->flags, NEB_TENDRIL_LEADING);

        //travel distance remaining
        distanceLeft = NEB_MAX_TENDRIL_DISTANCE - nebChunkDistance(chunka, chunkb);

        //repeat until we've gone far enough
        while (distanceLeft > 0.0f)
        {
            if (chunkb->numTendrils + 1 >= NEB_MAX_TENDRILS)
            {
                //too many tendrils on this guy, abort
                break;
            }

            //save the previous chunka
            lastchunk = chunka;
            //last endpoint becomes new startpoint
            chunka = chunkb;
            //get an endpoint, subject to constraints of previous chunk
            chunkb = nebPickChunk(neb, NEB_MEDIAN_CHUNK, lastchunk, chunka);
            if (chunkb != NULL)
            {
                nebChunkPicked(chunkb, TRUE);
                distanceLeft -= nebChunkDistance(chunka, chunkb);
                //try to get a new tendril for these chunks
                tendril = nebGetFreshTendril(neb);
                if (tendril != NULL)
                {
                    //got one, set it up
                    tendril->a = chunka;
                    tendril->b = chunkb;

                    lastTendril->next = tendril;
                    tendril->prev = lastTendril;
                    lastTendril = tendril;

                    nebAddTendril(chunka, tendril, NEB_CHUNK_FORE);
                    nebAddTendril(chunkb, tendril, NEB_CHUNK_AFT);
                    nebTendrilDirection(tendril);
                }
                else
                {
                    //couldn't find a tendril, so end this tendril and then
                    //the whole deal
                    distanceLeft = 0.0f;
                }
            }
            else
            {
                //couldn't find a suitable endpoint, so end this tendril
                distanceLeft = 0.0f;
            }
        }

        bitSet(lastTendril->flags, NEB_TENDRIL_TRAILING);
    }

    nebUnpickAllChunks(neb);
}

/*-----------------------------------------------------------------------------
    Name        : nebGo
    Description : creates chunks and tendrils; we call this a "nebula"
    Inputs      : origin - where the centre of the nebula is located in space
                  radius - maximum extent of the nebula about the origin
                  numOfChunks - the number of chunks that comprise the nebula
                  numOfTendrils - the number of tendrils that comprise the nebula
    Outputs     :
    Return      :
    Notes       : numOfChunks will probably be determined by a provided macro that
                  uses the radius and a density factor to cook up a value.  likewise
                  numOfTendrils wrt numOfChunks
----------------------------------------------------------------------------*/
void nebGo(vector* origin, vector* dimension, udword numOfChunks, udword numOfTendrils)
{
    nebulae_t* neb;

    neb = &nebNebulae[numNebulae++];

    nebAllocateChunks(neb, numOfChunks);
    nebAllocateTendrils(neb, numOfTendrils);
    nebDistributeChunks(neb, origin, dimension);
    nebAttachTendrils(neb);
    nebDeactivateEmptyChunksAndFixup(neb);
}

/*-----------------------------------------------------------------------------
    Name        : nebDeactivateEmptyChunksAndFixup
    Description : deactivate chunks with no attached tendrils, generate minimally
                  sized tables, and fixup data
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void nebDeactivateEmptyChunksAndFixup(nebulae_t* neb)
{
    udword i, t, hits;
    nebChunk* chunk;
    nebChunk* newTable;
    nebChunk* newChunk;
    nebTendril* tendril;

    //PHASE 1: set a flag on tendril-less chunks
    for (i = hits = 0, chunk = neb->chunkTable; i < neb->numChunks; i++, chunk++)
    {
        if (chunk->numTendrils == 0)
        {
            bitSet(chunk->flags, NEB_CHUNK_INACTIVE);
        }
        else
        {
            hits++;
        }
    }

    dbgAssert(hits != 0);

    //PHASE 2: create a new chunk structure that lacks bare chunks
    newTable = (nebChunk*)memAlloc(hits * sizeof(nebChunk), "nebulae chunks", 0);
    _alloc += hits * sizeof(nebChunk);

    for (i = 0, chunk = neb->chunkTable, newChunk = newTable; i < neb->numChunks; i++, chunk++)
    {
        if (!bitTest(chunk->flags, NEB_CHUNK_INACTIVE))
        {
            //valid chunk, so copy it over to the new table
            memcpy(newChunk, chunk, sizeof(nebChunk));

            //parent pointer
            newChunk->nebulae = (void*)neb;

            //now update all the tendrils
            for (t = 0, tendril = neb->tendrilTable; t < neb->numTendrils; t++, tendril++)
            {
                if (tendril->a == chunk)
                {
                    tendril->a = newChunk;
                }
                if (tendril->b == chunk)
                {
                    tendril->b = newChunk;
                }
            }

            //add a resource to the game
            (void)univAddNebula(Nebula0, &newChunk->position, (void*)newChunk);

            //goto next chunk in the new table
            newChunk++;
        }
    }

    //housekeeping
    neb->numChunks = hits;
    memFree(neb->chunkTable);
    neb->chunkTable = newTable;

    //PHASE 3: remove tendrils that are both leading and trailing
    for (i = 0, tendril = neb->tendrilTable; i < neb->numTendrils; i++, tendril++)
    {
        if (bitTest(tendril->flags, NEB_TENDRIL_LEADING) &&
            bitTest(tendril->flags, NEB_TENDRIL_TRAILING))
        {
            //flag 'em
            bitSet(tendril->flags, NEB_TENDRIL_INACTIVE);
        }
    }

    //PHASE 4: check to see if there's >1 tendril traversing the same 2 chunks
    for (i = hits = 0, tendril = neb->tendrilTable; i < neb->numTendrils; i++, tendril++)
    {
        nebTendril* otherTendril;
        udword j;

        for (j = 0, otherTendril = neb->tendrilTable; j < neb->numTendrils; j++, otherTendril++)
        {
            if (otherTendril == tendril)
            {
                continue;
            }
            if (tendril->a == otherTendril->a && tendril->b == otherTendril->b)
            {
                dbgMessagef("\ntendril == otherTendril (%d)", hits++);
            }
            if (tendril->a == otherTendril->b && tendril->b == otherTendril->a)
            {
                dbgMessagef("\ntendril ~= otherTendril (%d)", hits++);
            }
        }
    }

    //FIXME [PHASE 5]: fixup trailing/leading tendril flags if a tendril has
    //docked/departed from a chunk that has an/other tendril(s) attached
}

/*-----------------------------------------------------------------------------
    Name        : nebTransformNormal
    Description : transforms a normal from object -> world space
    Inputs      : vec - the target vector
                  m - the hmatrix that effects the object->world transform
                  nx, ny, nz - normal components
    Outputs     : vec - the final (unit) vector
    Return      :
----------------------------------------------------------------------------*/
void nebTransformNormal(vector* vec, real32* m, real32 nx, real32 ny, real32 nz)
{
#if 1
    vec->x = nx*m[0] + ny*m[1] + nz*m[2];
    vec->y = nx*m[4] + ny*m[5] + nz*m[6];
    vec->z = nx*m[8] + ny*m[9] + nz*m[10];
#else
    vec->x = nx;
    vec->y = ny;
    vec->z = nz;
#endif
    vecNormalize(vec);
}

/*-----------------------------------------------------------------------------
    Name        : nebTransformVert
    Description : transforms a vertex from object -> world space
    Inputs      : vecOut - the target vector
                  hmat - the hmatrix that effects the object->world transform
                  x, y, z - components of the source vector
    Outputs     : vecOut - the final (transformed) vector
    Return      :
----------------------------------------------------------------------------*/
void nebTransformVert(vector* vecOut, real32* hmat, real32 x, real32 y, real32 z)
{
    hvector hvecIn, hvecOut;

    hvecIn.x = x;
    hvecIn.y = y;
    hvecIn.z = z;
    hvecIn.w = 1.0f;
    hmatMultiplyHMatByHVec(&hvecOut, (hmatrix*)hmat, &hvecIn);
    nebHomogenize(&hvecOut);
    vecOut->x = hvecOut.x;
    vecOut->y = hvecOut.y;
    vecOut->z = hvecOut.z;
}

/*-----------------------------------------------------------------------------
    Name        : nebGetTendrilNormal
    Description : retrieves requested normal from a tendril's soup
    Inputs      : tendril - the tendril
                  whichNormal - normal number
                  lod - lod number
                  normal - space for the target
    Outputs     : normal - the requested normal
    Return      :
----------------------------------------------------------------------------*/
void nebGetTendrilNormal(nebTendril* tendril, sdword whichNormal, sdword lod, vector* normal)
{
    real32* np = tendril->lod[lod].normals;
    np += 3 * whichNormal;
    normal->x = np[0];
    normal->y = np[1];
    normal->z = np[2];
}

/*-----------------------------------------------------------------------------
    Name        : nebGetTendrilVert
    Description : retrieves requested vertex of a tendril
    Inputs      : tendril - the tendril
                  whichVert - vertex number
                  lod - lod number
                  vert - space for the target
    Outputs     : vert - the requested vertex
    Return      :
----------------------------------------------------------------------------*/
void nebGetTendrilVert(nebTendril* tendril, sdword whichVert, sdword lod, vector* vert)
{
    real32* vp = tendril->lod[lod].vertices;
    vp += 3 * whichVert;
    vert->x = vp[0];
    vert->y = vp[1];
    vert->z = vp[2];
}

/*-----------------------------------------------------------------------------
    Name        : nebColourAdjust
    Description :
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void nebColourAdjust(vector* vert, vector* norm, real32* m, real32* minv)
{
    if (usingShader)
    {
        ubyte color[4];

        color[0] = nebColor[0];
        color[1] = nebColor[1];
        color[2] = nebColor[2];
        color[3] = nebColor[3];
        shSpecularColour(1, 0, vert, norm, color, m, minv);
        glColor4ub(color[0], color[1], color[2], color[3]);
    }
}

/*-----------------------------------------------------------------------------
    Name        : nebDrawChunk2
    Description : draws a trivial joiner between the tendrils of a chunk Ot(2)
    Inputs      : chunk - the chunk
                  lod - lod of the chunk
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void nebDrawChunk2(nebChunk* chunk, sdword lod)
{
#define COLOUR(c) \
    { \
        GLubyte a = colAlpha(c); \
        if (!_bright) \
        { \
            a += TENDRILBRIGHTEN; \
        } \
        nebColor[0] = colRed(c); \
        nebColor[1] = colGreen(c); \
        nebColor[2] = colBlue(c); \
        nebColor[3] = a; \
        glColor4ub(nebColor[0], nebColor[1], nebColor[2], nebColor[3]); \
    }
    nebTendril* ta;
    nebTendril* tb;
    nebTendril* tendril;
    vector verta0, vertb0, verta1, vertb1;
    vector norma0, normb0, norma1, normb1;
    udword i, ia0, ib0, ia1, ib1;
    sdword a, b;
    color cola, colb;

    real32 m[16], minv[16];

    if (usingShader)
    {
        glGetFloatv(GL_MODELVIEW_MATRIX, m);
        shInvertMatrix(minv, m);
    }

    dbgAssert(chunk != NULL);

    a = b = -1;
    for (i = 0; i < chunk->numTendrils; i++)
    {
        tendril = (nebTendril*)chunk->tendrils[i];
        if (!bitTest(tendril->flags, NEB_TENDRIL_INACTIVE) &&
            tendril->lod[lod].numVerts > 0)
        {
            if (a == -1)
            {
                a = i;
            }
            else
            {
                b = i;
                break;
            }
        }
    }

    dbgAssert(a != -1);
    dbgAssert(b != -1);

    ta = (nebTendril*)chunk->tendrils[a];
    tb = (nebTendril*)chunk->tendrils[b];
    dbgAssert(ta != NULL);
    dbgAssert(tb != NULL);

    cola = ta->colour;
    colb = tb->colour;

    if (ta->lod[lod].numVerts == 0 || tb->lod[lod].numVerts == 0)
    {
        return;
    }

    for (i = 0; i < ta->lod[lod].slices; i++)
    {
        ia0 = ta->lod[lod].numVerts - ta->lod[lod].slices + i;
        ib0 = i;
        ia1 = ta->lod[lod].numVerts - ta->lod[lod].slices + ((i + 1) % ta->lod[lod].slices);
        ib1 = (ib0 + 1) % tb->lod[lod].slices;

        nebGetTendrilVert(ta, ia0, lod, &verta0);
        nebGetTendrilVert(tb, ib0, lod, &vertb0);
        nebGetTendrilVert(ta, ia1, lod, &verta1);
        nebGetTendrilVert(tb, ib1, lod, &vertb1);

        vecAddTo(verta0, chunk->dPos);
        vecAddTo(vertb0, chunk->dPos);
        vecAddTo(verta1, chunk->dPos);
        vecAddTo(vertb1, chunk->dPos);

        nebGetTendrilNormal(ta, ia0, lod, &norma0);
        nebGetTendrilNormal(tb, ib0, lod, &normb0);
        nebGetTendrilNormal(ta, ia1, lod, &norma1);
        nebGetTendrilNormal(tb, ib1, lod, &normb1);

        glBegin(GL_QUADS);

        COLOUR(cola);
        glNormal3f(norma0.x, norma0.y, norma0.z);
        nebColourAdjust(&verta0, &norma0, m, minv);
        glVertex3fv((GLfloat*)&verta0);

        COLOUR(colb);
        glNormal3f(normb0.x, normb0.y, normb0.z);
        nebColourAdjust(&vertb0, &normb0, m, minv);
        glVertex3fv((GLfloat*)&vertb0);

        COLOUR(colb);
        glNormal3f(normb1.x, normb1.y, normb1.z);
        nebColourAdjust(&vertb1, &normb1, m, minv);
        glVertex3fv((GLfloat*)&vertb1);

        COLOUR(cola);
        glNormal3f(norma1.x, norma1.y, norma1.z);
        nebColourAdjust(&verta1, &norma1, m, minv);
        glVertex3fv((GLfloat*)&verta1);

        glEnd();
    }
    alodIncPolys(ta->lod[lod].slices);
#undef COLOUR
}

/*-----------------------------------------------------------------------------
    Name        : nebRenderChunk
    Description : renders a chunk if it hasn't already been rendered
    Inputs      : chunk - the chunk
                  lod - lod number
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void nebRenderChunk(nebChunk* chunk, sdword lod)
{
    udword i, actualTendrils;
    nebTendril* tendril;

    dbgAssert(chunk != NULL);

    if (RGL && !usingShader) rglFeature(NEB_RENDER);

    TENDRILCOLOR0(TENDRILALPHA);

    if (chunk->counter < neb_frame_counter)
    {
        //render this guy, he hasn't been rendered yet this frame
        chunk->counter = neb_frame_counter;

        for (i = actualTendrils = 0; i < chunk->numTendrils; i++)
        {
            tendril = (nebTendril*)chunk->tendrils[i];
            if (!bitTest(tendril->flags, NEB_TENDRIL_INACTIVE) &&
                tendril->lod[lod].numVerts > 0)
            {
                actualTendrils++;
            }
        }

        switch (actualTendrils)
        {
        case 0:
        case 1:
            break;
        case 2:
            nebDrawChunk2(chunk, lod);
            break;
        }
    }

    if (RGL && !usingShader) rglFeature(RGL_NORMAL_RENDER);

    TENDRILCOLOR0(TENDRILALPHA);
}

/*-----------------------------------------------------------------------------
    Name        : nebDrawTendril
    Description : draws an already-allocated tendril
    Inputs      : tendril - the tendril to draw
                  lod - lod number
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void nebDrawTendril(nebTendril* tendril, sdword lod)
{
    udword i, j, t;
    vector vert, norm;
    vector dPosA, dPosB;

    real32 m[16], minv[16];

    if (usingShader)
    {
        glGetFloatv(GL_MODELVIEW_MATRIX, m);
        shInvertMatrix(minv, m);
    }

    dPosA = tendril->a->dPos;
    dPosB = tendril->b->dPos;

    if (RGL && !usingShader) rglFeature(NEB_RENDER);

    glBegin(GL_QUADS);
    for (j = 1; j <= tendril->lod[lod].stacks; j++)
    {
        for (i = 0; i < tendril->lod[lod].slices; i++)
        {
            nebGetTendrilNormal(tendril, (j-1)*tendril->lod[lod].slices + i, lod, &norm);
            glNormal3f(norm.x, norm.y, norm.z);
            nebGetTendrilVert(tendril, (j-1)*tendril->lod[lod].slices + i, lod, &vert);
            if (j == 1)
            {
                vecAddTo(vert, dPosA);
                if (bitTest(tendril->flags, NEB_TENDRIL_LEADING))
                {
                    TENDRILCOLOR(tendril,0);
                }
            }
            nebColourAdjust(&vert, &norm, m, minv);
            glVertex3fv((GLfloat*)&vert);
            TENDRILCOLOR(tendril,colAlpha(tendril->colour));

            nebGetTendrilNormal(tendril, j*tendril->lod[lod].slices + i, lod, &norm);
            glNormal3f(norm.x, norm.y, norm.z);
            nebGetTendrilVert(tendril, j*tendril->lod[lod].slices + i, lod, &vert);
            if (j == tendril->lod[lod].stacks)
            {
                vecAddTo(vert, dPosB);
                if (bitTest(tendril->flags, NEB_TENDRIL_TRAILING))
                {
                    TENDRILCOLOR(tendril,0);
                }
            }
            nebColourAdjust(&vert, &norm, m, minv);
            glVertex3fv((GLfloat*)&vert);
            TENDRILCOLOR(tendril,colAlpha(tendril->colour));

            t = (i+1) % tendril->lod[lod].slices;

            nebGetTendrilNormal(tendril, j*tendril->lod[lod].slices + t, lod, &norm);
            glNormal3f(norm.x, norm.y, norm.z);
            nebGetTendrilVert(tendril, j*tendril->lod[lod].slices + t, lod, &vert);
            if (j == tendril->lod[lod].stacks)
            {
                vecAddTo(vert, dPosB);
                if (bitTest(tendril->flags, NEB_TENDRIL_TRAILING))
                {
                    TENDRILCOLOR(tendril,0);
                }
            }
            nebColourAdjust(&vert, &norm, m, minv);
            glVertex3fv((GLfloat*)&vert);
            TENDRILCOLOR(tendril,colAlpha(tendril->colour));

            nebGetTendrilNormal(tendril, (j-1)*tendril->lod[lod].slices + t, lod, &norm);
            glNormal3f(norm.x, norm.y, norm.z);
            nebGetTendrilVert(tendril, (j-1)*tendril->lod[lod].slices + t, lod, &vert);
            if (j == 1)
            {
                vecAddTo(vert, dPosA);
                if (bitTest(tendril->flags, NEB_TENDRIL_LEADING))
                {
                    TENDRILCOLOR(tendril,0);
                }
            }
            nebColourAdjust(&vert, &norm, m, minv);
            glVertex3fv((GLfloat*)&vert);
            TENDRILCOLOR(tendril,colAlpha(tendril->colour));
        }
    }
    glEnd();
    alodIncPolys(tendril->lod[lod].stacks * tendril->lod[lod].slices);

    if (RGL && !usingShader) rglFeature(RGL_NORMAL_RENDER);
}

/*-----------------------------------------------------------------------------
    Name        : nebTendrilBounds
    Description : calculates a bounding sphere around a tendril
    Inputs      : tendril - the tendril
    Outputs     : modified the tendril structure elements corresponding to the bsphere
    Return      :
----------------------------------------------------------------------------*/
void nebTendrilBounds(nebTendril* tendril)
{
    sdword i;
    real32 dx, dy, dz;
    real32 rad_sq, xspan, yspan, zspan, maxspan;
    real32 old_to_p, old_to_p_sq, old_to_new;
    vector xmin, xmax, ymin, ymax, zmin, zmax, dia1, dia2;
    vector caller_p;
    real32 BIGNUMBER = 1E9f;
    real32 rad;
    vector cen;
    sdword lod = 1;

    xmin.x = ymin.y = zmin.z = BIGNUMBER;
    xmax.x = ymax.y = zmax.z = -BIGNUMBER;

    for (i = 0; i < (sdword)tendril->lod[lod].numVerts; i++)
    {
        nebGetTendrilVert(tendril, i, lod, &caller_p);

        if (caller_p.x < xmin.x) xmin = caller_p;
        if (caller_p.x > xmax.x) xmax = caller_p;
        if (caller_p.y < ymin.y) ymin = caller_p;
        if (caller_p.y > ymax.y) ymax = caller_p;
        if (caller_p.z < zmin.z) zmin = caller_p;
        if (caller_p.z > zmax.z) zmax = caller_p;
    }

    dx = xmax.x - xmin.x;
    dy = xmax.y - xmin.y;
    dz = xmax.z - xmin.z;
    xspan = dx*dx + dy*dy + dz*dz;

    dx = ymax.x - ymin.x;
    dy = ymax.y - ymin.y;
    dz = ymax.z - ymin.z;
    yspan = dx*dx + dy*dy + dz*dz;

    dx = zmax.x - zmin.x;
    dy = zmax.y - zmin.y;
    dz = zmax.z - zmin.z;
    zspan = dx*dx + dy*dy + dz*dz;

    dia1 = xmin;
    dia2 = xmax;
    maxspan = xspan;
    if (yspan > maxspan)
    {
        maxspan = yspan;
        dia1 = ymin;
        dia2 = ymax;
    }
    if (zspan > maxspan)
    {
        dia1 = zmin;
        dia2 = zmax;
    }

    cen.x = 0.5f * (dia1.x + dia2.x);
    cen.y = 0.5f * (dia1.y + dia2.y);
    cen.z = 0.5f * (dia1.z + dia2.z);

    dx = dia2.x - cen.x;
    dy = dia2.y - cen.y;
    dz = dia2.z - cen.z;
    rad_sq = dx*dx + dy*dy + dz*dz;
    rad = fsqrt(rad_sq);

    for (i = 0; i < (sdword)tendril->lod[lod].numVerts; i++)
    {
        nebGetTendrilVert(tendril, i, lod, &caller_p);

        dx = caller_p.x - cen.x;
        dy = caller_p.y - cen.y;
        dz = caller_p.z - cen.z;
        old_to_p_sq = dx*dx + dy*dy + dz*dz;
        if (old_to_p_sq > rad_sq)
        {
            old_to_p = fsqrt(old_to_p_sq);
            rad = 0.5f * (rad + old_to_p);
            rad_sq = rad*rad;
            old_to_new = old_to_p - rad;
            cen.x = (rad*cen.x + old_to_new*caller_p.x) / old_to_p;
            cen.y = (rad*cen.y + old_to_new*caller_p.y) / old_to_p;
            cen.z = (rad*cen.z + old_to_new*caller_p.z) / old_to_p;
        }
    }

    tendril->radius = rad;
    tendril->midpoint = cen;
}

/*-----------------------------------------------------------------------------
    Name        : nebCreateCylinder
    Description : creates the data necessary for rendering a tendril
    Inputs      : tendril - the tendril
                  hmat - the local -> world hmatrix for this tendril
                  baseRadius - radius at the base
                  topRadius - radius at the top
                  height - height/length of the tendril
                  slices - number of slices in the tendril
                  stacks - number of stacks in the tendril
                  lod - lod number
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void nebCreateCylinder(
    nebTendril* tendril, real32* hmat, real32 baseRadius, real32 topRadius,
    real32 height, sdword slices, sdword stacks, sdword lod)
{
    real32* vp;
    real32* np;
    sdword i, j;
    real32 da, dr, dz, nz, z, r;
    real32 x1, y1;
    real32 sign;
    vector vert, normal;

    if (bitTest(tendril->flags, NEB_TENDRIL_GEOMETRY))
    {
        return;
    }

    for (i = 0; i < 16; i++)
    {
        tendril->lod[lod].hmat[i] = hmat[i];
    }

    tendril->lod[lod].numVerts = slices * (stacks + 1);
    tendril->lod[lod].numFaces = 0;
    //3 components per vertex
    tendril->lod[lod].vertices = (real32*)memAlloc(tendril->lod[lod].numVerts*3*sizeof(real32), "tendril vertices", 0);
    _alloc += tendril->lod[lod].numVerts*3*sizeof(real32);
    //3 components per normal
    tendril->lod[lod].normals = (real32*)memAlloc(tendril->lod[lod].numVerts*3*sizeof(real32), "tendril normals", 0);
    _alloc += tendril->lod[lod].numVerts*3*sizeof(real32);
    tendril->lod[lod].faces = NULL;
    tendril->lod[lod].slices = slices;
    tendril->lod[lod].stacks = stacks;

    vp = tendril->lod[lod].vertices;
    np = tendril->lod[lod].normals;

    da = 2.0f * 3.14159265f / slices;
    dr = (topRadius - baseRadius) / slices;
    dz = height / stacks;
    nz = (baseRadius - topRadius) / height;

    z = 0.0f;

    if (ranRandom(RAN_Nebulae) % 2 == 0)
    {
        sign = 1.0f;
    }
    else
    {
        sign = -1.0f;
    }

    if (stacks == 5)
    {
        for (j = 0; j <= stacks; j++)
        {
            r = baseRadius;

            for (i = 0; i < slices; i++, vp += 3, np += 3)
            {
                x1 = -fsin(i * da);
                y1 = fcos(i * da);

                if (j == 2)
                {
                    real32 r2 = r + sign*r*0.15f;
                    nebTransformVert(&vert, hmat, x1*r2, y1*r2, z);
                }
                else if (j == 1 || j == 3)
                {
                    real32 r2 = r + sign*r*0.1f;
                    nebTransformVert(&vert, hmat, x1*r2, y1*r2, z);
                }
                else
                {
                    nebTransformVert(&vert, hmat, x1*r, y1*r, z);
                }
                *(vp+0) = vert.x;
                *(vp+1) = vert.y;
                *(vp+2) = vert.z;

                nebTransformNormal(&normal, hmat, x1, y1, nz);
                *(np+0) = normal.x;
                *(np+1) = normal.y;
                *(np+2) = normal.z;

                r += dr;
            }

            z += dz;
        }
    }
    else
    {
        for (j = 0; j <= stacks; j++)
        {
            r = baseRadius;

            for (i = 0; i < slices; i++, vp += 3, np += 3)
            {
                x1 = -fsin(i * da);
                y1 = fcos(i * da);

                nebTransformVert(&vert, hmat, x1*r, y1*r, z);
                *(vp+0) = vert.x;
                *(vp+1) = vert.y;
                *(vp+2) = vert.z;

                nebTransformNormal(&normal, hmat, x1, y1, nz);
                *(np+0) = normal.x;
                *(np+1) = normal.y;
                *(np+2) = normal.z;

                r += dr;
            }

            z += dz;
        }
    }

    nebTendrilBounds(tendril);
}

/*-----------------------------------------------------------------------------
    Name        : nebIndexOfNeb
    Description : returns the index of the particular nebulae structure
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
sdword nebIndexOfNeb(nebulae_t* neb)
{
    sdword i;

    for (i = 0; i < NEB_MAX_NEBULAE; i++)
    {
        if (neb == &nebNebulae[i])
        {
            return i;
        }
    }

    return -1;
}

/*-----------------------------------------------------------------------------
    Name        : nebNewNeb
    Description : callback from univupdate, sets a spaceobj ptr in the chunk
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void nebNewNeb(struct Nebula* neb)
{
    nebChunk* chunk = (nebChunk*)neb->stub;
    chunk->spaceobj = (void*)neb;
    neb->nebNebulaeIndex = nebIndexOfNeb(chunk->nebulae);
}

/*-----------------------------------------------------------------------------
    Name        : nebLOD
    Description : returns an lod given 2 chunks.  for tendrils.
    Inputs      : chunka, chunkb - the chunks
    Outputs     :
    Return      : 0 or 1, representing the lod number
----------------------------------------------------------------------------*/
sdword nebLOD(nebChunk* chunka, nebChunk* chunkb)
{
    bool a = univSpaceObjInRenderList((SpaceObj*)chunka->spaceobj);
    bool b = univSpaceObjInRenderList((SpaceObj*)chunkb->spaceobj);

    return (a || b) ? 0 : 1;
}

/*-----------------------------------------------------------------------------
    Name        : nebRenderTendril
    Description : renders a tendril
    Inputs      : tendril - the tendril
                  lod - lod number
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void nebRenderTendril(nebTendril* tendril, sdword lod)
{
    vector a, b, direction;

    dbgAssert(tendril != NULL);
    dbgAssert(tendril->a != NULL);
    dbgAssert(tendril->b != NULL);
    dbgAssert(tendril->a != tendril->b);

    if (tendril->counter >= neb_frame_counter)
    {
        return;
    }
    tendril->counter = neb_frame_counter;

    a = tendril->a->position;
    b = tendril->b->position;

    //leave room for the chunk to grow towards this tendril

    direction = tendril->direction;
    vecMultiplyByScalar(direction, NEB_SHUFFLE_DISTANCE);
    vecAddTo(a, direction);

    direction = tendril->direction;
    vecMultiplyByScalar(direction, NEB_SHUFFLE_DISTANCE);
    vecSubFrom(b, direction);

/*
    want a cylinder around the tendril's ideal line.
    a->b is forward vector of tendril's coordinate system.
    'z' is up vector of coordinate system.
    a->b X 'z' is right vector of coordinate system.
 */

    {
        vector forward, up, right;
        vector a0, b0;
        real32 length;

        a0 = a;
        b0 = b;

        vecSub(forward, b, a);
        length = fsqrt(vecMagnitudeSquared(forward));
        vecNormalize(&forward);
        direction = forward;

        vecSet(up, 0.0f, 0.0f, 1.0f);
        vecCrossProduct(right, forward, up);

        if (!bitTest(tendril->flags, NEB_TENDRIL_GEOMETRY))
        {
            real32 hmat[16];
            real32 radius, rand;

            hmat[0] = right.x;
            hmat[1] = right.y;
            hmat[2] = right.z;
            hmat[3] = 0.0f;
            hmat[4] = up.x;
            hmat[5] = up.y;
            hmat[6] = up.z;
            hmat[7] = 0.0f;
            hmat[8] = forward.x;
            hmat[9] = forward.y;
            hmat[10] = forward.z;
            hmat[11] = 0.0f;
            hmat[12] = a0.x;
            hmat[13] = a0.y;
            hmat[14] = a0.z;
            hmat[15] = 1.0f;

            rand = realRand(NEB_TENDRIL_RADIUS_RANGE);
            if (ranRandom(RAN_Nebulae) % 2 == 0)
            {
                rand = -rand;
            }
            radius = NEB_TENDRIL_RADIUS_BASE + rand;

            nebCreateCylinder(tendril, hmat, radius, radius, length, 6, 5, 0);
            nebCreateCylinder(tendril, hmat, radius, radius, length, 6, 1, 1);

            bitSet(tendril->flags, NEB_TENDRIL_GEOMETRY);
        }

        nebDrawTendril(tendril, lod);
    }
}

/*-----------------------------------------------------------------------------
    Name        : nebDistanceFromCamera
    Description : returns the distance from the current cameraposition
    Inputs      : pos - position in worldspace
    Outputs     :
    Return      : scalar distance from camera
----------------------------------------------------------------------------*/
real32 nebDistanceFromCamera(vector* pos)
{
    vector dist;

    vecSub(dist, *pos, mrCamera->eyeposition);
    return fsqrt(vecMagnitudeSquared(dist));
}

/*-----------------------------------------------------------------------------
    Name        : nebDistanceFromCameraSquared
    Description : returns the distance squared from the current cameraposition
    Inputs      : pos - position in worldspace
    Outputs     :
    Return      : scalar distance squared from camera
----------------------------------------------------------------------------*/
real32 nebDistanceFromCameraSquared(vector* pos)
{
    vector dist;

    vecSub(dist, *pos, mrCamera->eyeposition);
    return vecMagnitudeSquared(dist);
}


/*-----------------------------------------------------------------------------
    Name        : nebColorDarkenAndDesaturate
    Description : darkens and desaturates a color by provided amounts
    Inputs      : c - the color to modify
                  dark - [0..255] amount to darken
                  desat - [0..255] amount to desaturate
    Outputs     :
    Return      : modified color value
----------------------------------------------------------------------------*/
sdword nebColorDarkenAndDesaturate(color c, sdword dark, sdword desat)
{
    real32 h, s, v;
    real32 red = colUbyteToReal(colRed(c));
    real32 green = colUbyteToReal(colGreen(c));
    real32 blue = colUbyteToReal(colBlue(c));

    colRGBToHSV(&h, &s, &v, red, green, blue);

    v -= colUbyteToReal(dark);
    if (v < 0.0f) v = 0.0f;
    s -= colUbyteToReal(desat);
    if (s < 0.0f) s = 0.0f;

    colHSVToRGB(&red, &green, &blue, h, s, v);

    return colRGBA(colRealToUbyte(red), colRealToUbyte(green), colRealToUbyte(blue),
                   colAlpha(c));
}

/*-----------------------------------------------------------------------------
    Name        : nebColourTendril
    Description : colours a tendril based on distance
    Inputs      : tendril - the tendril to colour
                  lod - lod number of the tendril
    Outputs     : sets tendril->colour to a modified (by 1/d) tendril->realColour
    Return      :
----------------------------------------------------------------------------*/
void nebColourTendril(nebTendril* tendril, sdword lod)
{
    real32 dA, dB, d;
    static real32 MAXD = 40000.0f;

    dbgAssert(tendril != NULL);
#if 0
    tendril->colour = tendril->realColour;
#else
    dA = nebDistanceFromCamera(&tendril->a->position);
    dB = nebDistanceFromCamera(&tendril->b->position);

    if (dA > MAXD) dA = MAXD;
    if (dB > MAXD) dB = MAXD;

    d = 0.5f * (dA + dB);
    d /= MAXD;
    d *= NEB_DISTANCE_DESATURATION;

    tendril->colour = nebColorDarkenAndDesaturate(tendril->realColour,
                                                  (sdword)(0.75f * d), (sdword)d);
    tendril->colour = colRGBA(colRed(tendril->colour), colGreen(tendril->colour),
                              colBlue(tendril->colour),
                              (ubyte)(tendril->fadeFactor * (real32)colAlpha(tendril->colour)));
#endif
}

/*-----------------------------------------------------------------------------
    Name        : nebIsClipped
    Description : determines whether to render a tendril and its chunks
    Inputs      : tendril - the tendril
    Outputs     :
    Return      : TRUE or FALSE, depending
----------------------------------------------------------------------------*/
bool nebIsClipped(nebTendril* tendril)
{
#if 0
    //FIXME: dammit, this is buggy.  holy moley it's fast, though
    real32 r = 1.5f * tendril->radius;
    return rglIsClipped((GLfloat*)&tendril->midpoint, r, r, r);
#else
    vector veye, vobj;
    vecSub(veye, mrCamera->eyeposition, mrCamera->lookatpoint);
    vecSub(vobj, mrCamera->eyeposition, tendril->midpoint);
    return (vecDotProduct(veye, vobj) < 0.0f);
#endif
}

/*-----------------------------------------------------------------------------
    Name        : nebRender
    Description : renders the nebulae
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void nebRender(void)
{
    sdword i;

    for (i = 0; i < numNebulae; i++)
    {
        nebRenderNebula(&nebNebulae[i]);
    }
}

/*-----------------------------------------------------------------------------
    Name        : nebRenderNebula
    Description : renders a nebula
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void nebRenderNebula(nebulae_t* neb)
{
    udword i;
    nebChunk* chunk;
    nebTendril* tendril;
    bool fogOn, atOn, cullOff;
    real32 maxDist;

    if (neb->numTendrils == 0 || smSensorsActive)
    {
        return;
    }

    _bright = glCapFastFeature(GL_BLEND);

    fogOn = glIsEnabled(GL_FOG);
    atOn = glIsEnabled(GL_ALPHA_TEST);
    cullOff = !glIsEnabled(GL_CULL_FACE);

    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_FALSE);
    if (fogOn) glDisable(GL_FOG);

    if (RGL && !usingShader)
    {
        rndLightingEnable(TRUE);
    }
    else
    {
        rndLightingEnable(FALSE);
    }

    rndTextureEnable(FALSE);
    glEnable(GL_BLEND);
    rndAdditiveBlends(FALSE);
    glDepthMask(GL_FALSE);
    if (atOn) glDisable(GL_ALPHA_TEST);
    if (cullOff) glEnable(GL_CULL_FACE);

    TENDRILCOLOR0(TENDRILALPHA);

    neb_frame_counter++;
    if (neb_frame_counter == 0)
    {
        //overflow has occurred, fix it
        neb_frame_counter = 1;
        for (i = 0, chunk = neb->chunkTable; i < neb->numChunks; i++, chunk++)
        {
            chunk->counter = 0;
        }
        for (i = 0, tendril = neb->tendrilTable; i < neb->numTendrils; i++, tendril++)
        {
            tendril->counter = 0;
        }
    }

    maxDist = 1.1f * mrCamera->clipPlaneFar;
    maxDist *= maxDist;     //squared

    for (i = 0, tendril = neb->tendrilTable; i < neb->numTendrils; i++, tendril++)
    {
        sdword lod;

        //check for activity
        if (!bitTest(tendril->flags, NEB_TENDRIL_INACTIVE))
        {
            if (nebIsClipped(tendril))
            {
                continue;
            }

            lod = nebLOD(tendril->a, tendril->b);
            nebColourTendril(tendril, lod);

            //too far to be visible?
            if ((nebDistanceFromCameraSquared(&tendril->a->position) > maxDist) &&
                (nebDistanceFromCameraSquared(&tendril->b->position) > maxDist))
            {
                continue;
            }

            if (!bitTest(tendril->flags, NEB_TENDRIL_LEADING))
            {
                //render chunka if this is not a leading tendril
                nebRenderChunk(tendril->a, lod);
            }
            if (!bitTest(tendril->flags, NEB_TENDRIL_TRAILING))
            {
                //render chunkb if this is not a trailing tendril
                nebRenderChunk(tendril->b, lod);
            }

            //render the tendril itself
            nebRenderTendril(tendril, lod);
        }
    }

    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);

    if (fogOn) glEnable(GL_FOG);
    if (atOn) glEnable(GL_ALPHA_TEST);
    if (cullOff) glDisable(GL_CULL_FACE);

    if (usingShader)
    {
        rndLightingEnable(TRUE);
    }
}

//MUST also remove itself from the resource list, &c
void nebDeleteChunkSimply(nebChunk* chunk)
{
    dbgAssert(chunk != NULL);
    bitSet(chunk->flags, NEB_CHUNK_DEAD);
}

/*-----------------------------------------------------------------------------
    Name        : nebRemoveTendril
    Description : wipes out a tendril after a chunk in its link has been harvested
    Inputs      : tendril - the tendril
                  chunk - the chunk that was deleted causing our deletion
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
/*
 * this fn handles the following conditions
 *    o the tendril is a leading tendril, in which case it is set inactive and
 *      any chunks on it (that aren't the passed chunk) are told of the departure
 *    o the tendril is a trailing tendril, in which case it is set inactive and
 *      any chunks on it (that aren't the passed chunk) are told of the departure
 *    o the tendril is a median tendril, in which case nothing yet happens
 */
void nebRemoveTendril(nebTendril* tendril, nebChunk* chunk)
{
#if 0
    if (bitSet(tendril->flags, NEB_TENDRIL_LEADING) ||
        bitSet(tendril->flags, NEB_TENDRIL_TRAILING))
    {
        if (tendril->a != chunk)
        {
            nebDeleteChunkSimply(tendril->a);
        }
        if (tendril->b != chunk)
        {
            nebDeleteChunkSimply(tendril->b);
        }
    }
    else
    {
        dbgAssert(tendril != NULL);
        dbgAssert(chunk != NULL);
    }
#else
    bitSet(tendril->flags, NEB_TENDRIL_INACTIVE);

    if (tendril->prev != NULL)
    {
        bitSet(tendril->prev->flags, NEB_TENDRIL_TRAILING);
    }
    if (tendril->next != NULL)
    {
        bitSet(tendril->next->flags, NEB_TENDRIL_LEADING);
    }
#endif
}

/*-----------------------------------------------------------------------------
    Name        : nebDeleteChunk
    Description : handles a chunk that has been resourced to death
    Inputs      : chunk - the chunk
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void nebDeleteChunk(nebChunk* chunk)
{
    udword i;
    nebulae_t* neb;

    if (numNebulae == 0)
    {
        //already deleted
        return;
    }

    neb = (nebulae_t*)chunk->nebulae;

    dbgAssert(neb != NULL);

    if (neb->numChunks == 0)
    {
        //already deleted
        return;
    }

    dbgAssert(chunk != NULL);
    bitSet(chunk->flags, NEB_CHUNK_DEAD);

    for (i = 0; i < chunk->numTendrils; i++)
    {
        nebRemoveTendril(chunk->tendrils[i], chunk);
    }
}

/*-----------------------------------------------------------------------------
    Name        : nebFadeAttachedTendrils
    Description : fade tendrils attached to a chunk that's being resourced
    Inputs      : chunk - the chunk
                  factor - [0..1] fraction of maximum RU's
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
/*
 * issues:
 *    o should fade only endpoints of a tendril actually attached to the chunk
 *    o chunks should fade at the attachment point
 */
void nebFadeAttachedTendrils(nebChunk* chunk, real32 factor)
{
    udword i;
    nebulae_t* neb;

    neb = (nebulae_t*)chunk->nebulae;

    dbgAssert(neb != NULL);

    if (neb->numChunks == 0)
    {
        //nothing to fade - what's going on?
        dbgFatal(DBG_Loc, "numChunks == 0 in nebFadeAttachedTendrils");
    }

    dbgAssert(chunk != NULL);

    for (i = 0; i < chunk->numTendrils; i++)
    {
        nebTendril* tendril;

        tendril = chunk->tendrils[i];
        tendril->fadeFactor = factor;
    }
}

/*-----------------------------------------------------------------------------
    Name        : nebUpdateChunk
    Description : adds wiggle, sloosh to a chunk
    Inputs      : chunk - the chunk to move around
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
/*
 * a chunk picks a random direction to move in and just keeps going until it
 * has exceeded the bounds of its outerRadius, in which case it picks another
 * direction and goes on about its business
 */
void nebUpdateChunk(nebChunk* chunk)
{
    dbgAssert(chunk != NULL);

    //add velocity to dPos
    vecAddTo(chunk->dPos, chunk->velocity);

    //check magnitude
    if (fsqrt(vecMagnitudeSquared(chunk->dPos)) > (chunk->outerRadius - chunk->innerRadius))
    {
        //from whence we came
        vecSubFrom(chunk->dPos, chunk->velocity);
        //go somewhere new
        chunkRandomVelocity(chunk);
    }
}

static real32 nebradius = NEB_CHUNK_RADIUS*2.0f;

/*-----------------------------------------------------------------------------
    Name        : nebSetFog
    Description : decides what to do about fog
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void nebSetFog()
{
    Node* node;
    SpaceObj* spaceobj;
    real32 sumOf = 0.0f, dist;
    real32 MIN_SUMOF = 0.1f;
    real32 MAX_SUMOF = 0.57f;

    if (dontNebulate || numNebulae == 0)
    {
        //nothing to do, or we're not allowed to do anything
        return;
    }

    //gather linear proximity info
    node = universe.RenderList.head;
    while (node != NULL)
    {
        spaceobj = (SpaceObj*)listGetStructOfNode(node);
        if (spaceobj->objtype == OBJ_NebulaType)
        {
            dist = fsqrt(spaceobj->cameraDistanceSquared);
            if (dist != 0.0f)
            {
                sumOf += nebradius / dist;
            }

            nebUpdateChunk((nebChunk*)((Nebula*)spaceobj)->stub);
        }

        node = node->next;
    }

    sumOf *= 0.15f;
    nebFogSum = sumOf;
    if (sumOf > 0.86f)
    {
        sumOf = 0.86f;
    }

    if (sumOf > 0.1f)
    {
        rndFogOn = TRUE;
        glFogfv(GL_FOG_COLOR, nebFogColor);
        glFogf(GL_FOG_DENSITY, sumOf);
    }
    else
    {
        rndFogOn = FALSE;
    }

    if (!smSensorsActive)
    {
        if (sumOf > 0.0f)
        {
            int r, g, b;
            sumOf *= 1.5f;
            if (sumOf > MAX_SUMOF) sumOf = MAX_SUMOF;
            else if (sumOf < MIN_SUMOF) sumOf = MIN_SUMOF;
            nebFogSum = sumOf;
            if (!_bright) sumOf *= 1.4f;
            r = (int)(NEB_FOG_RED * sumOf * NEB_BG_SCALAR * 255.0f);
            g = (int)(NEB_FOG_GREEN * sumOf * NEB_BG_SCALAR * 255.0f);
            b = (int)(NEB_FOG_BLUE * sumOf * NEB_BG_SCALAR * 255.0f);
            r += colRed(universe.backgroundColor);
            g += colGreen(universe.backgroundColor);
            b += colBlue(universe.backgroundColor);
            if (r > 255) r = 255;
            if (g > 255) g = 255;
            if (b > 255) b = 255;
            rndSetClearColor(colRGBA(r,g,b,255));
        }
    }
}

/*=============================================================================
    Save functions for nebula
=============================================================================*/

#pragma warning( 4 : 4047)      // turns off "different levels of indirection warning"

sdword nebChunkPtrToNum(nebulae_t* neb, nebChunk *nebChunk)
{
    sdword i;
    for (i=0;i<neb->numChunks;i++)
    {
        if (nebChunk == &neb->chunkTable[i])
        {
            return i;
        }
    }

    return -1;
}

nebChunk *nebNumToChunkPtr(nebulae_t* neb, sdword num)
{
    if (num == -1)
    {
        return NULL;
    }
    else
    {
        dbgAssert(num < neb->numChunks);
        return &neb->chunkTable[num];
    }
}

sdword nebTendrilPtrToNum(nebulae_t* neb, nebTendril *nebTendril)
{
    sdword i;

    for (i=0;i<neb->numTendrils;i++)
    {
        if (nebTendril == &neb->tendrilTable[i])
        {
            return i;
        }
    }

    return -1;
}

nebTendril *nebNumToTendrilPtr(nebulae_t* neb, sdword num)
{
    if (num == -1)
    {
        return NULL;
    }
    else
    {
        dbgAssert(num < neb->numTendrils);
        return &neb->tendrilTable[num];
    }
}

void nebSave_nebTendrilLOD(nebTendrilLOD *nebtendrilLOD)
{
    if (nebtendrilLOD->vertices != NULL)
    {
        SaveChunk *chunk = CreateChunk(BASIC_STRUCTURE,sizeof(real32)*nebtendrilLOD->numVerts*3,nebtendrilLOD->vertices);
        SaveThisChunk(chunk);
        memFree(chunk);
    }

    if (nebtendrilLOD->normals != NULL)
    {
        SaveChunk *chunk = CreateChunk(BASIC_STRUCTURE,sizeof(real32)*nebtendrilLOD->numVerts*3,nebtendrilLOD->normals);
        SaveThisChunk(chunk);
        memFree(chunk);
    }

    dbgAssert(nebtendrilLOD->faces == NULL);
}

void nebSave_nebTendril(nebulae_t* neb, nebTendril *nebtendril)
{
    sdword i;
    SaveChunk *chunk;
    nebTendril *savecontents;

    chunk = CreateChunk(BASIC_STRUCTURE|SAVE_NEBTENDRIL,sizeof(nebTendril),nebtendril);
    savecontents = chunkContents(chunk);

    savecontents->a = nebChunkPtrToNum(neb, savecontents->a);
    savecontents->b = nebChunkPtrToNum(neb, savecontents->b);

    savecontents->prev = nebTendrilPtrToNum(neb, savecontents->prev);
    savecontents->next = nebTendrilPtrToNum(neb, savecontents->next);

    SaveThisChunk(chunk);
    memFree(chunk);

    for (i=0;i<NUM_NEBTENDRIL_LODS;i++)
    {
        nebSave_nebTendrilLOD(&nebtendril->lod[i]);
    }
}

void nebSave_nebChunk(nebulae_t* neb, nebChunk *nebchunk)
{
    sdword i;
    SaveChunk *chunk;
    nebChunk *savecontents;

    chunk = CreateChunk(BASIC_STRUCTURE|SAVE_NEBCHUNK,sizeof(nebChunk),nebchunk);
    savecontents = chunkContents(chunk);

    for (i=0;i<NEB_MAX_TENDRILS;i++)
    {
        savecontents->tendrils[i] = nebTendrilPtrToNum(neb, savecontents->tendrils[i]);
    }

    savecontents->spaceobj = SpaceObjRegistryGetID(savecontents->spaceobj);
    savecontents->nebulae = nebIndexOfNeb(neb);

    SaveThisChunk(chunk);
    memFree(chunk);
}

void nebSave_Nebula(void)
{
    sdword i, nebIndex;
    nebulae_t* neb;

    SaveInfoNumber(neb_frame_counter);

    SaveInfoNumber(numNebulae);

    for (nebIndex = 0; nebIndex < numNebulae; nebIndex++)
    {
        neb = &nebNebulae[nebIndex];

        SaveInfoNumber(neb->numChunks);

        for (i=0;i<neb->numChunks;i++)
        {
            nebSave_nebChunk(neb, &neb->chunkTable[i]);
        }

        SaveInfoNumber(neb->numTendrils);

        for (i=0;i<neb->numTendrils;i++)
        {
            nebSave_nebTendril(neb, &neb->tendrilTable[i]);
        }
    }
}

void nebLoad_nebChunk(nebulae_t* neb, nebChunk *nebchunk)
{
    SaveChunk *chunk;

    chunk = LoadNextChunk();
    VerifyChunk(chunk,BASIC_STRUCTURE|SAVE_NEBCHUNK,sizeof(nebChunk));

    memcpy(nebchunk,chunkContents(chunk),sizeof(nebChunk));
    memFree(chunk);
}

void nebFix_nebChunk(nebulae_t* neb, nebChunk *nebchunk)
{
    sdword i;

    for (i=0;i<NEB_MAX_TENDRILS;i++)
    {
        nebchunk->tendrils[i] = nebNumToTendrilPtr(neb, (sdword)nebchunk->tendrils[i]);
    }

    nebchunk->spaceobj = SpaceObjRegistryGetObj((sdword)nebchunk->spaceobj);
    nebchunk->nebulae = neb;
}

void nebLoad_nebTendrilLOD(nebulae_t* neb, nebTendrilLOD *nebtendrilLOD)
{
    if (nebtendrilLOD->vertices != NULL)
    {
        SaveChunk *chunk = LoadNextChunk();
        sdword size = sizeof(real32)*nebtendrilLOD->numVerts*3;
        VerifyChunk(chunk,BASIC_STRUCTURE,size);

        nebtendrilLOD->vertices = memAlloc(size, "tendril vertices", 0);
        memcpy(nebtendrilLOD->vertices,chunkContents(chunk),size);
        memFree(chunk);
    }

    if (nebtendrilLOD->normals != NULL)
    {
        SaveChunk *chunk = LoadNextChunk();
        sdword size = sizeof(real32)*nebtendrilLOD->numVerts*3;
        VerifyChunk(chunk,BASIC_STRUCTURE,size);

        nebtendrilLOD->normals = memAlloc(size, "tendril normals", 0);
        memcpy(nebtendrilLOD->normals,chunkContents(chunk),size);
        memFree(chunk);
    }

    dbgAssert(nebtendrilLOD->faces == NULL);
}

void nebLoad_nebTendril(nebulae_t* neb, nebTendril *nebtendril)
{
    SaveChunk *chunk;
    sdword i;

    chunk = LoadNextChunk();
    VerifyChunk(chunk,BASIC_STRUCTURE|SAVE_NEBTENDRIL,sizeof(nebTendril));

    memcpy(nebtendril,chunkContents(chunk),sizeof(nebTendril));
    memFree(chunk);

    for (i=0;i<NUM_NEBTENDRIL_LODS;i++)
    {
        nebLoad_nebTendrilLOD(neb, &nebtendril->lod[i]);
    }
}

void nebFix_nebTendril(nebulae_t* neb, nebTendril *nebtendril)
{
    nebtendril->a = nebNumToChunkPtr(neb, (sdword)nebtendril->a);
    nebtendril->b = nebNumToChunkPtr(neb, (sdword)nebtendril->b);

    nebtendril->prev = nebNumToTendrilPtr(neb, (sdword)nebtendril->prev);
    nebtendril->next = nebNumToTendrilPtr(neb, (sdword)nebtendril->next);
}

void nebLoad_Nebula(void)
{
    sdword i, nebIndex;
    nebulae_t* neb;

    nebColorInit();

    neb_frame_counter = LoadInfoNumber();

    numNebulae = LoadInfoNumber();

    for (nebIndex = 0; nebIndex < numNebulae; nebIndex++)
    {
        neb = &nebNebulae[nebIndex];

        neb->numChunks = LoadInfoNumber();
        if (neb->numChunks != 0)
        {
            neb->chunkTable = memAlloc(neb->numChunks * sizeof(nebChunk), "nebula chunkTable", 0);
        }

        for (i=0;i<neb->numChunks;i++)
        {
            nebLoad_nebChunk(neb, &neb->chunkTable[i]);
        }

        neb->numTendrils = LoadInfoNumber();
        if (neb->numTendrils != 0)
        {
            neb->tendrilTable = memAlloc(neb->numTendrils * sizeof(nebTendril), "nebula tendrilTable", 0);
        }

        for (i=0;i<neb->numTendrils;i++)
        {
            nebLoad_nebTendril(neb, &neb->tendrilTable[i]);
        }

        // now fix everything up

        for (i=0;i<neb->numChunks;i++)
        {
            nebFix_nebChunk(neb, &neb->chunkTable[i]);
        }

        for (i=0;i<neb->numTendrils;i++)
        {
            nebFix_nebTendril(neb, &neb->tendrilTable[i]);
        }
    }
}

#pragma warning( 2 : 4047)      // turn back on "different levels of indirection warning"

