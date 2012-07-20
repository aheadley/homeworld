/*=============================================================================
    Name    : Mex.h
    Purpose : Definitions for Mex.c (Mesh extensions)

    Created 7/18/1997 by gshaw
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifndef ___MEX_H
#define ___MEX_H

#include "Types.h"
#include "Vector.h"
#include "SpaceObj.h"

typedef struct tagMEXFileHeader
{
    char            identifier[8];                  // File identifier.
    unsigned short  version;                        // File version.
    unsigned short  nChunks;                        // Number of chunks in the file.
} MEXFileHeader;

typedef struct tagMEXChunk
{
    // Generic stuff.
    char            type[4];
    char            name[8];
    unsigned long   chunkSize;
} MEXChunk;

typedef struct tagMEXGunChunk
{
    // Generic stuff.
    char            type[4];
    char            name[8];
    unsigned long   chunkSize;

    // Specific stuff...
    vector          position;
    vector          normal;

    real32          coneAngle;
    real32          edgeAngle;
} MEXGunChunk;

typedef struct tagMEXEngineChunk
{
    // Generic stuff.
    char            type[4];
    char            name[8];
    unsigned long   chunkSize;

    // Specific stuff...
    vector          position;
    vector          normal;

    real32          coneAngle;
    real32          edgeAngle;
} MEXEngineChunk;

typedef struct tagMEXCollisionSphereChunk
{
    // Generic stuff.
    char            type[4];
    char            name[8];
    unsigned long   chunkSize;

    // Specific stuff...
    udword          level;

    vector          offset;
    real32          r;
} MEXCollisionSphereChunk;


typedef struct tagMEXCollisionRectangleChunk
{
    // Generic stuff.
    char            type[4];
    char            name[8];
    unsigned long   chunkSize;

    // Specific stuff...
    udword          level;

    float           ox;
    float           oy;
    float           oz;

    float           dx;
    float           dy;
    float           dz;
} MEXCollisionRectangleChunk;


typedef struct tagMEXDockingChunk
{
    // Generic stuff.
    char            type[4];
    char            name[8];            // Always "DOCK"
    unsigned long   chunkSize;

    // Specific stuff...
    vector          position;
    vector          normal;

    real32          coneAngle;
    real32          edgeAngle;

    char            dockName[20];       // Actual name.
} MEXDockingChunk;

typedef struct tagMEXSalvageChunk
{
    // Generic stuff.
    char            type[4];
    char            name[8];            // Always "SALVAGE"
    unsigned long   chunkSize;

    // Specific stuff...
    vector          position;
    vector          normal;

    real32          coneAngle;
    real32          edgeAngle;

    char            Name[20];       // Actual name.
} MEXSalvageChunk;

typedef struct tagMEXNAVLightChunk
{
    // Generic stuff.
    char            type[4];
    char            name[8];            // Always "NAV"
    udword          chunkSize;

    // Specific stuff...
    vector          position;

    unsigned char   red;
    unsigned char   blue;
    unsigned char   green;
    unsigned char   pad;

    char            NAVLightName[20];       // Actual name.
} MEXNAVLightChunk; // 36 bytes...

void *mexGetChunk(void *mex,char *type,char *name);

void *mexLoad(char *filename);
void mexFree(void *mex);
bool mexVerify(void *mex);

ResNozzleStatic *mexGetResNozzleStatic(void *mex);
RepairNozzleStatic *mexGetRepairNozzleStatic(void *mex);
TractorBeamStatic *mexGetTractorBeamStatic(void *mex);
void mexGetGunStaticInfo(GunStaticInfo *gunstaticinfo,void *mex);
void mexGetDockStaticInfo(DockStaticInfo *dockstaticinfo,void *mex);
void mexGetSalvageStaticInfo(SalvageStaticInfo *salvagestaticinfo,void *mex);
void mexGetNAVLightStaticInfo(NAVLightStaticInfo *navlightstaticinfo, void *mex);
void mexSetCollisionInfo(StaticCollInfo *staticCollInfo,void *mex,real32 forwardScale,real32 upScale,real32 rightScale, real32 xslide, real32 yslide, real32 zslide);
void mexSetEngineNozzle(vector* engineNozzleOffset, void* mex, sdword index);
void mexVecToVec(vector *dest, vector *source);
MEXDockingChunk *mexGetDockChunk(void *mex,char *dockname);
MEXSalvageChunk *mexGetSalChunk(void *mex,char *salvagename);
MEXNAVLightChunk *mexGetNAVLightChunk(void *mex,char *navLightName);

#endif

